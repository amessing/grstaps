(define (problem ijrr_3)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street43 street52 - street
		building13 building22 building26 building28 - building
	)
	(:init
		(atLocation survivor0 street52)
		(atLocation survivor1 building26)
		(onFire building13)
		(onFire building28)
		(underRubble street43)
		(underRubble building22)
		(needsRepair building13)
		(needsRepair building22)
		(needsRepair building28)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street43))
		(not (needsRepair building13))
		(not (needsRepair building22))
		(not (needsRepair building28))
	))
	(:metric minimize total-time)
)
